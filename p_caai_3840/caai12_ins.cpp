/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:能源采集信息新增
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai12_ins)
//-EP_SYSTEM_HEAD_END

int f_caai12_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	
	CModel tcaaia12("TCAAIA12");

	CString		datetime = " ";
	CString  sqlstr("");
	

	CDbCommand cmd_inq(conn);
	
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//将对象字段重置为默认值
			tcaaia12.Reset();

			// 获取前台传入参数
			tcaaia12.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaaia12["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tcaaia12["REC_CREATOR"] = s.userid;
			if (tcaaia12["DEPT_CODE"].ToString().Trim() == "")
			{
				sprintf(s.msg, "产线不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tcaaia12.QueryCount("DEPT_CODE, PROD_DATE, SHIFT_NO, SHIFT_GROUP, MAT_CODE, EQU_NO, DEV_POS, BUSI_TYPE") != 0)
			{
				sprintf(s.msg, "生产日期[%s]能源信息已经存在!", (const char*)tcaaia12["PROD_DATE"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			tcaaia12.Insert();  

		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		 
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
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}