/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 备用表使用新增
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaib1_ins)
int f_caaib1_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	int i =0;

	// 定义表的实体对象
	CModel tcaaib1("TCAAIB1");


	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		// 传入块中第一个表的行数
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//将对象字段重置为默认值
			tcaaib1.Reset();

			// 获取前台传入参数
			tcaaib1.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaaib1["REC_CREATOR"] = s.userid;
			tcaaib1["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");			

			sqlstr = "select  @project_id||decode (key_seq,'','000001',trim(to_char(to_number(nvl(key_seq,0))+1, '000000'))) from ("
				" select max(substr(key_seq,length(@project_id)+1,6)) key_seq"
				" from tcaaib1"
				" where 1=1"
				" and project_id = @project_id"
				")"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("project_id", tcaaib1["PROJECT_ID"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaaib1["KEY_SEQ"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			if (tcaaib1["KEY_SEQ"].ToString().Trim()=="")
			{ tcaaib1["KEY_SEQ"] = tcaaib1["PROJECT_ID"].ToString() + "000001"; }

			// 执行新增,失败抛出异常			
			tcaaib1.Insert();
		}


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}

