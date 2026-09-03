/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 物料工序使用配置修改
**************************************************/
#include "stdafx.h"

// Service 入口
BM2F_ENTERACE(caac14_upd)

int f_caac14_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	// 定义表的实体对象
	CModel tcaac14("TCAAC14");

	try
	{
		

		// 传入块中第一个表的行数
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for( int i = 0; i < rowCount;  i++ )
		{
		    //将对象字段重置为默认值
			tcaac14.Reset();

		    // 获取前台传入参数
		    tcaac14.MergeFrom(bcls_rec->Tables[0].Rows[i]);

            //修改一条记录
			tcaac14["REC_REVISOR"]     = s.userid;
			tcaac14["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

            tcaac14.Update("REC_REVISOR,REC_REVISE_TIME,ITEM_CNAME,CODE,FIELD_NAME,SCHE_NO","SUB_BACKLOG_CODE");

		}

		//设置系统返回消息，国际化信息
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { "TCAAC16", ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000009")/*修改记录失败，表[{0}]，sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch(const CApplicationException& ex)
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

	return(doFlag);
}