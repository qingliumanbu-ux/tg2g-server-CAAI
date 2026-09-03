/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 工序成本基础信息的查询
**************************************************/


#include "stdafx.h"


// service入口
BM2F_ENTERACE(caac_inq)
//-EP_SYSTEM_HEAD_END
int f_caac_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{

	/*定义函数名*/
	CTracer log(__FUNCTION__);


	/*程序用变量*/
	int doFlag = 0;
	int fetchRowCount;
	
	CString  sqlstr = "";
	CString   type_code= "";
	CString   form_id = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);	
	 
	try
	{	
		
		
		form_id = bcls_rec->Tables[0].Rows[0]["FORM_ID"].ToString();
		if (form_id != "")
		{
			bcls_ret->Tables[0].set_TableName("RECV_DEPT");
				sqlstr = " SELECT code,code_desc_1_content from tep0002 t1"
					" WHERE 1=1"
					" AND EXISTS (SELECT 1 FROM  TEP0002 t2 WHERE instr(t2.CODE_DESC_1_CONTENT,t1.CODE)>0 "
					" AND t2.code=@userid and t2.code_class = 'CAU1')"
					" and code_class = 'CAW4'"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("userid", s.userid);
				cmd_inq.ExecuteQuery(bcls_ret->Tables["RECV_DEPT"]);
				cmd_inq.Close();

				bcls_ret->Tables.Add("OUT_UNIT_CODE");
				sqlstr = " SELECT code,code_desc_1_content from tep0002 t1"
					" WHERE 1=1"
					" AND EXISTS (SELECT 1 FROM  TEP0002 t2 WHERE instr(t2.CODE_DESC_2_CONTENT,t1.CODE)>0 "
					" AND t2.code=@userid and t2.code_class = 'CAU1')"
					" and code_class = 'CAW3'"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("userid", s.userid);
				cmd_inq.ExecuteQuery(bcls_ret->Tables["OUT_UNIT_CODE"]);
				cmd_inq.Close();
		}
		

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
